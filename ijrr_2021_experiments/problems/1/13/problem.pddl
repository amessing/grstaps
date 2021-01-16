(define (problem ijrr_13)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street13 street55 - street
		building1 building7 building35 building36 - building
	)
	(:init
		(atLocation survivor0 street13)
		(atLocation survivor1 building35)
		(onFire building7)
		(onFire building36)
		(underRubble street55)
		(underRubble building1)
		(needsRepair building1)
		(needsRepair building7)
		(needsRepair building36)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street55))
		(not (needsRepair building1))
		(not (needsRepair building7))
		(not (needsRepair building36))
	))
	(:metric minimize total-time)
)
