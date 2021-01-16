(define (problem ijrr_5)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street10 street48 - street
		building17 building18 building29 building33 - building
	)
	(:init
		(atLocation survivor0 street48)
		(atLocation survivor1 building17)
		(onFire building18)
		(onFire building29)
		(underRubble street10)
		(underRubble building33)
		(needsRepair building18)
		(needsRepair building29)
		(needsRepair building33)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street10))
		(not (needsRepair building18))
		(not (needsRepair building29))
		(not (needsRepair building33))
	))
	(:metric minimize total-time)
)
