(define (problem ijrr_10)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street31 street49 - street
		building4 building10 building23 building36 - building
	)
	(:init
		(atLocation survivor0 street49)
		(atLocation survivor1 building4)
		(onFire building10)
		(onFire building36)
		(underRubble street31)
		(underRubble building23)
		(needsRepair building10)
		(needsRepair building23)
		(needsRepair building36)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street31))
		(not (needsRepair building10))
		(not (needsRepair building23))
		(not (needsRepair building36))
	))
	(:metric minimize total-time)
)
